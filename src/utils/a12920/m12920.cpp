#include "a12920/m12920.h"
QVector<double> m12920::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
