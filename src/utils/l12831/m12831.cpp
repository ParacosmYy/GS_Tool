#include "l12831/m12831.h"
QVector<double> m12831::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
