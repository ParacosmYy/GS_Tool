#include "l36831/m36831.h"
QVector<double> m36831::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
