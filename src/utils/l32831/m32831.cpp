#include "l32831/m32831.h"
QVector<double> m32831::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
