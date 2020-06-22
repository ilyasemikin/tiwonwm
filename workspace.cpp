#include "workspace.hpp"

#include <algorithm>

using namespace std;

Workspace::Workspace(xcb_connection_t *connection) :
    connection_(connection)
{

}

void Workspace::AddWindow(xcb_window_t w_id) {
    windows_.push_back(w_id);
}

void Workspace::RemoveWindow(xcb_window_t w_id) {
    auto finded = find(begin(windows_), end(windows_), w_id);
    if (finded != end(windows_)) {
        windows_.erase(finded);
    }
}

bool Workspace::Has(xcb_window_t w_id) {
    return find(begin(windows_), end(windows_), w_id) != end(windows_);
}