

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


#define TRUE   1
#define FALSE  0
#define PORT 8889

using namespace std;
using namespace gphoto2pp;

const string ISO_CONFIG_NAME = "ISO Speed";
const string SHUTTER_SPEED_CONFIG_NAME = "Shutter Speed";
const string WHITEBALANCE_CONFIG_NAME = "WhiteBalance";

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
    } catch (exception e) {
        cerr << "exception " << e.what() << endl;
    }
    return {};
}

string getValue(string input) {
    string result[2];
// set iso 200
// set shutter_speed *&
    bool isStartFound = false;
    string value;
    int k = 0;
    for (int i = 0; i < input.size(); i++) {
        if (input[i] == '*') {
            isStartFound = true;
            continue;
        }

        if (isStartFound) {
            if (input[i] == '&') {
                return value;
            }
            value += input[i];
        }
    }
    return value;
}

string getInput(char buffer[], int actualSize) {
    string in;
    for (int i = 0; i < actualSize; i++) {
        in += buffer[i];
    }
    return in;
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
                        //set the string terminating NULL byte on the end of the data read
                        //buffer[valread] = '\0';
                        string input = getInput(buffer, valread);
                        input.replace(input.find("\r\n"), 2, "");

                        CameraWrapper &cameraWrapper = cameraWrappers.at(0);
                        CameraWrapper *cameraWrapper_ptr = &cameraWrapper;
                        string response;
                        if (input == "get_iso") {
                            future<string> iso_conf_future = async(launch::async, getRadioWidgetCurrentValueByName,
                                                                   cameraWrapper_ptr,
                                                                   ISO_CONFIG_NAME);
                            response = iso_conf_future.get();
                        } else if (input == "get_shutter_speed") {
                            future<string> shutter_speed_conf_future = async(launch::async,
                                                                             getRadioWidgetCurrentValueByName,
                                                                             cameraWrapper_ptr,
                                                                             SHUTTER_SPEED_CONFIG_NAME);
                            response = shutter_speed_conf_future.get();
                        } else if (input == "get_white_balance") {
                            future<string> white_balance_conf_future = async(launch::async,
                                                                             getRadioWidgetCurrentValueByName,
                                                                             cameraWrapper_ptr,
                                                                             WHITEBALANCE_CONFIG_NAME);
                            response = white_balance_conf_future.get();
                        } else if (input.find("set") != -1) {
                            if (input.find("iso") != -1) {
                                string value = getValue(input);
                                future<string>
                                        change_conf_future = async(launch::async, setRadioWidgetValueByName,
                                                                   cameraWrapper_ptr,
                                                                   ISO_CONFIG_NAME, value);
                                response = "200 ok";
                            } else if (input.find("shutter_speed") != -1) {
                                string value = getValue(input);
                                future<string>
                                        change_conf_future = async(launch::async, setRadioWidgetValueByName,
                                                                   cameraWrapper_ptr,
                                                                   SHUTTER_SPEED_CONFIG_NAME, value);
                                response = "200 ok";
                            } else if (input.find("white_balance") != -1) {
                                string value = getValue(input);
                                auto rootWidget = cameraWrapper.getConfig();
                                future<string>
                                        change_conf_future = async(launch::async, setRadioWidgetValueByName,
                                                                   cameraWrapper_ptr,
                                                                   WHITEBALANCE_CONFIG_NAME, value);
                                response = "200 ok";
                            } else {
                                response = "422 error";
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