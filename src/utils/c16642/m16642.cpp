#include "c16642/m16642.h"
QVector<double> m16642::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
