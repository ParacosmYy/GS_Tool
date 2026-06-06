#include "a15920/m15920.h"
QVector<double> m15920::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
