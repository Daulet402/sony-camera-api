/** \file 
 * \author Copyright (c) 2013 maldworth <https://github.com/maldworth>
 *
 * \note
 * This file is part of gphoto2pp
 * 
 * \note
 * gphoto2pp is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * \note
 * gphoto2pp is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * \note
 * You should have received a copy of the GNU Lesser General Public
 * License along with gphoto2pp.
 * If not, see http://www.gnu.org/licenses
 */

#include <gphoto2pp/helper_gphoto2.hpp>
#include <gphoto2pp/camera_list_wrapper.hpp>
#include <gphoto2pp/exceptions.hpp>
#include <gphoto2pp/camera_wrapper.hpp>
#include <gphoto2pp/window_widget.hpp>
#include <gphoto2pp/camera_widget_wrapper.hpp>

#include <iostream>
#include <gphoto2pp/radio_widget.hpp>
#include <boost/tokenizer.hpp>


using namespace std;
using namespace boost;
using namespace gphoto2pp;
const string ISO_CONFIG_NAME = "ISO Speed";
const string SHUTTER_SPEED_CONFIG_NAME = "Shutter Speed";
const string WHITEBALANCE_CONFIG_NAME = "WhiteBalance";

vector<string> getChoices(gphoto2pp::CameraWrapper &cameraWrapper, string widgetName) {
    /**
     * available only for ISO Speed, WhiteBalance
     */
    auto radioWidget = cameraWrapper.getConfig().getChildByLabel<gphoto2pp::RadioWidget>(widgetName);
    return radioWidget.getChoices();
}

/*string getRadioWidgetCurrentValueByName(const CameraWrapper &cameraWrapper, string widgetName) {
    auto radioWidget = cameraWrapper.getConfig().getChildByLabel<gphoto2pp::RadioWidget>(widgetName);
    return radioWidget.getValue();
}*/

void setRadioWidgetValueByName(gphoto2pp::WindowWidget &rootWidget, string widgetName, string value) {
    auto radioWidget = rootWidget.getChildByLabel<gphoto2pp::RadioWidget>(widgetName);
    radioWidget.setValue(value);
}

void updateRootConfig(gphoto2pp::CameraWrapper &cameraWrapper, gphoto2pp::WindowWidget &rootWidget) {
    rootWidget = cameraWrapper.getConfig();
}


gphoto2pp::CameraWrapper *getCameraWrapperArray() {
    try {
        auto cameraList = gphoto2pp::autoDetectAll();
        gphoto2pp::CameraWrapper cameraWrapperArray[cameraList.count()];
        for (int i = 0; i < cameraList.count(); ++i) {
            string model = cameraList.getPair(i).first;
            string port = cameraList.getPair(i).second;
            cameraWrapperArray[i] = gphoto2pp::CameraWrapper(model, port);
        }
        return cameraWrapperArray;
    }
    catch (const gphoto2pp::exceptions::NoCameraFoundError &e) {
        cout << "GPhoto couldn't detect any cameras connected to the computer" << endl;
        cout << "Exception Message: " << e.what() << endl;
        return nullptr;
    }
}

vector<gphoto2pp::CameraWrapper> getCameraWrappers_old() {
    try {
        auto cameraList = gphoto2pp::autoDetectAll();
        // vector<gphoto2pp::CameraWrapper> cameraWrappers(cameraList.count());

        for (int i = 0; i < cameraList.count(); ++i) {
            string model = cameraList.getPair(i).first;
            string port = cameraList.getPair(i).second;
            auto cameraWrapper = gphoto2pp::CameraWrapper(model, port);
            //cameraWrappers[i] = gphoto2pp::CameraWrapper(model, port);
            auto rootWidget = cameraWrapper.getConfig();
        }

        // auto cameraList = gphoto2pp::autoDetectAll();
        /*for (int i = 0; i < cameraList.count(); ++i) {
            string model = cameraList.getPair(i).first;
            string port = cameraList.getPair(i).second;
            auto cameraWrapper = gphoto2pp::CameraWrapper(model, port);
            cout << "" << endl;
        }*/
        //return cameraWrappers;
    }
    catch (const gphoto2pp::exceptions::NoCameraFoundError &e) {
        cout << "GPhoto couldn't detect any cameras connected to the computer" << endl;
        cout << "Exception Message: " << e.what() << endl;
    }
    return vector<gphoto2pp::CameraWrapper>(0);
}

int addTwoNums(int a, int b) {

    return a + b;
}

vector<CameraWrapper> getCameraWrappers() {
    try {
        auto cameraList = gphoto2pp::autoDetectAll();
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
    catch (const gphoto2pp::exceptions::NoCameraFoundError &e) {
        cout << "GPhoto couldn't detect any cameras connected to the computer" << endl;
        cout << "Exception Message: " << e.what() << endl;
    } catch (std::exception e2) {
        cerr << "exception " << e2.what() << endl;
    }
    return {};
}


int main(int argc, char *argv[]) {

    typedef tokenizer<char_separator<char>> tokenizer;
    string s = "Boost C++ Libraries";
    char_separator<char> sep{" "};

    tokenizer tok{s, sep};
    for (const auto &t : tok)
        cout << t << '\n';



    /*const vector<CameraWrapper> vector = getCameraWrappers();
    for (int i = 0; i < vector.size(); i++) {
        const CameraWrapper &cameraWrapper = vector.at(i);
        //cout << getRadioWidgetCurrentValueByName(cameraWrapper, ISO_CONFIG_NAME) << endl;
    }*/

    //delete vector;

    //cout << *addTwoNums_ptr(2, 5) << endl;
    /*int i[4] = {1, 2, 3, 4};
    int *i_ptr = i;


    for (int j = 0; j < 4; j++) {
        int **i_ptr_ptr = &i_ptr;
        cout << **i_ptr_ptr << endl;
        cout << *(i_ptr + j) << endl;
    }*/
    //cout << *i_ptr << endl;
    //cout << **i_ptr_ptr << endl;

    //cout << *i_ptr_ptr << endl;

    // runs autodetect method and returns all cameras connected to the computer

    // vector<gphoto2pp::CameraWrapper> wrappers = getCameraWrappers();
    //  vector<gphoto2pp::CameraWrapper *> cameraWrappers(1);
/*    try {
        auto cameraList = gphoto2pp::autoDetectAll();

        for (int i = 0; i < cameraList.count(); ++i) {
            string model = cameraList.getPair(i).first;
            string port = cameraList.getPair(i).second;

            auto cameraWrapper = gphoto2pp::CameraWrapper(model, port);
            auto rootWidget = cameraWrapper.getConfig();

            cout << getRadioWidgetCurrentValueByName(cameraWrapper, ISO_CONFIG_NAME) << endl;

            setRadioWidgetValueByName(rootWidget, ISO_CONFIG_NAME, "200");

            cameraWrapper.setConfig(rootWidget);
        }
    }
    catch (const gphoto2pp::exceptions::NoCameraFoundError &e) {
        cout << "GPhoto couldn't detect any cameras connected to the computer" << endl;
        cout << "Exception Message: " << e.what() << endl;
        return -1;
    }*/

    return 0;
}
