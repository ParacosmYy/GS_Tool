#include "l21831/m21831.h"
QVector<double> m21831::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
