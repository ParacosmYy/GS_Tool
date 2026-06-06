#include "l16831/m16831.h"
QVector<double> m16831::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
