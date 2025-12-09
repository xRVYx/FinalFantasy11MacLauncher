#pragma once

#include <QMainWindow>
#include "../core/Config.hpp"

QMainWindow* createMainWindow(Config& cfg, QWidget* parent = nullptr);
