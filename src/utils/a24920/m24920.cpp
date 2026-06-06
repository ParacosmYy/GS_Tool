#include "a24920/m24920.h"
QVector<double> m24920::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
