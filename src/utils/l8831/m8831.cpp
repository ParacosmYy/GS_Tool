#include "l8831/m8831.h"
QVector<double> m8831::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
