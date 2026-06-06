#include "l25831/m25831.h"
QVector<double> m25831::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
