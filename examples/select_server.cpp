

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <iostream>
#include <camera_methods.h>
#include <future>

#include <gphoto2pp/helper_gphoto2.hpp>
#include <gphoto2pp/camera_list_wrapper.hpp>
#include <gphoto2pp/exceptions.hpp>
#include <gphoto2pp/camera_wrapper.hpp>
#include <gphoto2pp/window_widget.hpp>
#include <gphoto2pp/camera_widget_wrapper.hpp>
#include <gphoto2pp/radio_widget.hpp>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>

#define TRUE   1
#define FALSE  0
#define PORT 8889

using namespace std;
using namespace boost;
using namespace gphoto2pp;

const string ISO_CONFIG_NAME = "ISO Speed";
const string SHUTTER_SPEED_CONFIG_NAME = "Shutter Speed";
const string WHITEBALANCE_CONFIG_NAME = "WhiteBalance";
const string DELIMITER = "_";

vector<string> getChoices(CameraWrapper &cameraWrapper, string widgetName) {
    /**
     * available only for ISO Speed, WhiteBalance
     */
    auto radioWidget = cameraWrapper.getConfig().getChildByLabel<RadioWidget>(widgetName);
    return radioWidget.getChoices();
}

string getRadioWidgetCurrentValueByName(CameraWrapper *cameraWrapper, string widgetName) {
    cout << "getRadioWidgetCurrentValueByName called " << endl;
    auto radioWidget = cameraWrapper->getConfig().getChildByLabel<RadioWidget>(widgetName);
    return radioWidget.getValue();
}

string setRadioWidgetValueByName(CameraWrapper *cameraWrapper, string widgetName, string value) {
    auto rootWidget = cameraWrapper->getConfig();
    auto radioWidget = rootWidget.getChildByLabel<RadioWidget>(widgetName);
    radioWidget.setValue(value);
    cameraWrapper->setConfig(rootWidget);
    return "ok";
}

void updateRootConfig(CameraWrapper &cameraWrapper, WindowWidget &rootWidget) {
    rootWidget = cameraWrapper.getConfig();
}

vector<CameraWrapper> getCameraWrappers() {
    try {
        auto cameraList = autoDetectAll();
        int count = cameraList.count();
        vector<CameraWrapper> cameraWrapperPtr;
        for (int i = 0; i < count; ++i) {
            string model = cameraList.getPair(i).first;
            string port = cameraList.getPair(i).second;
            //auto cameraWrapper = CameraWrapper(model, port);
            cameraWrapperPtr.push_back(CameraWrapper(model, port));
        }
        return cameraWrapperPtr;
    }
    catch (const exceptions::NoCameraFoundError &e) {
        cout << "GPhoto couldn't detect any cameras connected to the computer" << endl;
        cout << "Exception Message: " << e.what() << endl;
    } catch (std::exception e) {
        cerr << "exception " << e.what() << endl;
    }
    return {};
}

vector<string> getParameters(string input) {
    vector<string> params;
    typedef tokenizer<char_separator<char>> tokenizer;
    char_separator<char> sep("_");
    tokenizer tok{input, sep};
    for (const auto &t : tok) {
        if (t == "set") {
            continue;
        }
        params.push_back(t);
    }
    return params;
}

vector<string> get_key_and_value(string input, string replace_str) {
    replace_first(input, replace_str, "");
    typedef tokenizer<char_separator<char>> tokenizer;
    char_separator<char> sep("&");
    tokenizer tok{input, sep};
    vector<string> params;
    for (const auto &t : tok) {
        params.push_back(t);
    }
    return params;
}

vector<string> get_all_params(string input, string replace_str) {
    replace_first(input, replace_str, "");
    typedef tokenizer<char_separator<char>> tokenizer;
    char_separator<char> sep("_");
    tokenizer tok{input, sep};
    vector<string> params;
    for (const auto &t : tok) {
        params.push_back(t);
    }
    return params;
}

int main(int argc, char *argv[]) {
    vector<CameraWrapper> cameraWrappers = getCameraWrappers();
    int opt = TRUE;
    int master_socket, addrlen, new_socket, client_socket[30], max_clients = 30, activity, i, valread, sd;
    int max_sd;
    struct sockaddr_in address;

    char buffer[4096];

    fd_set readfds;

    const char *message = "ECHO Daemon v1.0 \r\n";

    for (i = 0; i < max_clients; i++) {
        client_socket[i] = 0;
    }

    if ((master_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *) &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(master_socket, (struct sockaddr *) &address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    printf("Listener on port %d \n", PORT);

    if (listen(master_socket, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    addrlen = sizeof(address);
    puts("Waiting for connections ...");

    try {
        while (true) {
            FD_ZERO(&readfds);
            FD_SET(master_socket, &readfds);
            max_sd = master_socket;

            for (i = 0; i < max_clients; i++) {
                sd = client_socket[i];
                if (sd > 0)
                    FD_SET(sd, &readfds);

                if (sd > max_sd)
                    max_sd = sd;
            }

            activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

            if ((activity < 0) && (errno != EINTR)) {
                printf("select error");
            }

            if (FD_ISSET(master_socket, &readfds)) {
                if ((new_socket = accept(master_socket, (struct sockaddr *) &address, (socklen_t *) &addrlen)) <
                    0) {
                    perror("accept");
                    exit(EXIT_FAILURE);
                }

                printf("New connection , socket fd is %d , ip is : %s , port : %d \n", new_socket,
                       inet_ntoa(address.sin_addr), ntohs(address.sin_port));

                /*if (send(new_socket, message, strlen(message), 0) != strlen(message)) {
                    perror("send");
                }*/


                for (i = 0; i < max_clients; i++) {
                    if (client_socket[i] == 0) {
                        client_socket[i] = new_socket;
                        printf("Adding to list of sockets as %d\n", i);

                        break;
                    }
                }
            }

            for (i = 0; i < max_clients; i++) {
                sd = client_socket[i];

                if (FD_ISSET(sd, &readfds)) {

                    if ((valread = read(sd, buffer, 1024)) == 0) {
                        getpeername(sd, (struct sockaddr *) &address, (socklen_t *) &addrlen);
                        printf("Host disconnected , ip %s , port %d \n",
                               inet_ntoa(address.sin_addr),
                               ntohs(address.sin_port));

                        close(sd);
                        client_socket[i] = 0;
                    } else {
                        string input(buffer, buffer + valread);
                        input.replace(input.find("\r\n"), 2, "");

                        CameraWrapper &cameraWrapper = cameraWrappers.at(0);
                        CameraWrapper *cameraWrapper_ptr = &cameraWrapper;
                        string response;

                        if (input == "get_all_cameras") {
                            for (CameraWrapper &wrapper :cameraWrappers) {
                                response += wrapper.getModel() + "&" + wrapper.getPort() + "&0;";
                            }
                            response = "ok_" + response.substr(0, response.size() - 1);
                        } else if (input.find("getisovalues_") != -1) {
                            vector<string> model_and_port = get_key_and_value(input, "getisovalues_");
                            for (CameraWrapper &wrapper :cameraWrappers) {
                                if (model_and_port.size() == 2 && wrapper.getModel() == model_and_port[0]
                                    && wrapper.getPort() == model_and_port[1]) {
                                    vector<string> choices = getChoices(wrapper, ISO_CONFIG_NAME);
                                    response = boost::algorithm::join(choices, "&");
                                    break;
                                }
                            }
                            if (response != "") {
                                response = "ok_" + response;
                            } else {
                                response = "error_empty response";
                            }
                        } else if (input.find("getwhitebalancevalues_") != -1) {
                            vector<string> model_and_port = get_key_and_value(input, "getwhitebalancevalues_");
                            for (CameraWrapper &wrapper :cameraWrappers) {
                                if (model_and_port.size() == 2 && wrapper.getModel() == model_and_port[0]
                                    && wrapper.getPort() == model_and_port[1]) {
                                    vector<string> choices = getChoices(wrapper, WHITEBALANCE_CONFIG_NAME);
                                    response = boost::algorithm::join(choices, "&");
                                    break;
                                }
                            }
                            if (response != "") {
                                response = "ok_" + response;
                            } else {
                                response = "error_empty response";
                            }
                        } else if (input.find("getsettings_") != -1) {
                            vector<string> model_and_port = get_key_and_value(input, "getsettings_");
                            for (CameraWrapper &wrapper :cameraWrappers) {
                                if (model_and_port.size() == 2 && wrapper.getModel() == model_and_port[0]
                                    && wrapper.getPort() == model_and_port[1]) {

                                    future<string> iso_conf_future = async(launch::async,
                                                                           getRadioWidgetCurrentValueByName,
                                                                           cameraWrapper_ptr,
                                                                           ISO_CONFIG_NAME);

                                    future<string> shutter_speed_conf_future = async(launch::async,
                                                                                     getRadioWidgetCurrentValueByName,
                                                                                     cameraWrapper_ptr,
                                                                                     SHUTTER_SPEED_CONFIG_NAME);

                                    future<string> white_balance_conf_future = async(launch::async,
                                                                                     getRadioWidgetCurrentValueByName,
                                                                                     cameraWrapper_ptr,
                                                                                     WHITEBALANCE_CONFIG_NAME);


                                    response = "iso&" + iso_conf_future.get() + ";"
                                               + "shutterspeed&" + shutter_speed_conf_future.get() + ";"
                                               + "whitebalance&" + white_balance_conf_future.get() + ";"
                                               + "focal&" + ";"
                                               + "whitebalancecelvin&" + ";"
                                               + "recordingstatus&" + ";"
                                               + "memorystatus&" + ";";

                                    if (response != "") {
                                        response = "ok_" + response;
                                    } else {
                                        response = "error_empty response";
                                    }
                                    break;
                                }
                            }

                        } else if (input.find("set_") != -1) {
                            vector<string> all_params = get_all_params(input, "set_");
                            vector<string> model_and_port = get_key_and_value(all_params[0], "");
                            cout << "model = " << model_and_port[0] << " port = " << model_and_port[1] << endl;

                            for (CameraWrapper &wrapper :cameraWrappers) {
                                if (model_and_port.size() == 2 && wrapper.getModel() == model_and_port[0]
                                    && wrapper.getPort() == model_and_port[1]) {

                                    CameraWrapper *wrapper_ptr = &cameraWrapper;
                                    for (int i = 1; i < all_params.size(); i++) {
                                        vector<string> key_and_value = get_key_and_value(all_params[i], "");
                                        if (key_and_value.size() == 2) {
                                            string key = key_and_value[0];
                                            string value = key_and_value[1];

                                            if (key == "iso") {
                                                future<string>
                                                        change_conf_future = async(launch::async,
                                                                                   setRadioWidgetValueByName,
                                                                                   wrapper_ptr,
                                                                                   ISO_CONFIG_NAME, value);

                                            } else if (key == "shutterspeed") {
                                                future<string>
                                                        change_conf_future = async(launch::async,
                                                                                   setRadioWidgetValueByName,
                                                                                   wrapper_ptr,
                                                                                   SHUTTER_SPEED_CONFIG_NAME, value);

                                            } else if (key == "whitebalance") {
                                                future<string>
                                                        change_conf_future = async(launch::async,
                                                                                   setRadioWidgetValueByName,
                                                                                   wrapper_ptr,
                                                                                   WHITEBALANCE_CONFIG_NAME, value);
                                            }
                                        }
                                    }
                                    wrapper_ptr = nullptr;
                                    break;
                                }
                            }

                            response = "ok";
                        } else if (input == "get_iso") {
                            future<string> iso_conf_future = async(launch::async, getRadioWidgetCurrentValueByName,
                                                                   cameraWrapper_ptr,
                                                                   ISO_CONFIG_NAME);
                            response = iso_conf_future.get();
                        } else if (input == "get_shutterspeed") {
                            future<string> shutter_speed_conf_future = async(launch::async,
                                                                             getRadioWidgetCurrentValueByName,
                                                                             cameraWrapper_ptr,
                                                                             SHUTTER_SPEED_CONFIG_NAME);
                            response = shutter_speed_conf_future.get();
                        } else if (input == "get_whitebalance") {
                            future<string> white_balance_conf_future = async(launch::async,
                                                                             getRadioWidgetCurrentValueByName,
                                                                             cameraWrapper_ptr,
                                                                             WHITEBALANCE_CONFIG_NAME);
                            response = white_balance_conf_future.get();
                        } else if (input.find("set") != -1) {
                            typedef tokenizer<char_separator<char>> tokenizer;
                            vector<string> params = getParameters(input);
                            if (params.size() == 2) {
                                if (params[0] == "iso") {
                                    string value = params[1];
                                    future<string>
                                            change_conf_future = async(launch::async, setRadioWidgetValueByName,
                                                                       cameraWrapper_ptr,
                                                                       ISO_CONFIG_NAME, value);
                                    response = "200 ok";
                                } else if (params[0] == "shutterspeed") {
                                    string value = params[1];
                                    future<string>
                                            change_conf_future = async(launch::async, setRadioWidgetValueByName,
                                                                       cameraWrapper_ptr,
                                                                       SHUTTER_SPEED_CONFIG_NAME, value);
                                    response = "200 ok";
                                } else if (params[0] == "whitebalance") {
                                    string value = params[1];
                                    auto rootWidget = cameraWrapper.getConfig();
                                    future<string>
                                            change_conf_future = async(launch::async, setRadioWidgetValueByName,
                                                                       cameraWrapper_ptr,
                                                                       WHITEBALANCE_CONFIG_NAME, value);
                                    response = "200 ok";
                                } else {
                                    response = "ERROR_002: No settings found";
                                }
                            } else {
                                response = "ERROR_001: Invalid command";
                            }

                        } else if (input == "set_white_balance") {
                            //  response = getRadioWidgetCurrentValueByName(cameraWrapper, WHITEBALANCE_CONFIG_NAME);
                        } else if (input == "set_white_balance") {
                            // response = getRadioWidgetCurrentValueByName(cameraWrapper, WHITEBALANCE_CONFIG_NAME);
                        } else {
                            response = "Invalid command";
                        }
                        cameraWrapper_ptr = nullptr;
                        response += "\r\n";
                        size_t length = response.copy(buffer, response.size(), 0);
                        buffer[length] = '\0';
                        send(sd, buffer, strlen(buffer), 0);
                    }


                }
            }
        }
    } catch (const exceptions::NoCameraFoundError &e) {
        cout << "GPhoto couldn't detect any cameras connected to the computer" << endl;
        cout << "Exception Message: " << e.what() << endl;
        return -1;
    }
    delete &cameraWrappers;
    return 0;
}