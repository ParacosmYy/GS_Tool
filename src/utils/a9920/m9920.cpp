#include "a9920/m9920.h"
QVector<double> m9920::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
