#include "a32920/m32920.h"
QVector<double> m32920::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
