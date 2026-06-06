#include "l9831/m9831.h"
QVector<double> m9831::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
