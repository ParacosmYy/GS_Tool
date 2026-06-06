#include "l24831/m24831.h"
QVector<double> m24831::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
