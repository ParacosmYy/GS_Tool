#include "l20831/m20831.h"
QVector<double> m20831::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
