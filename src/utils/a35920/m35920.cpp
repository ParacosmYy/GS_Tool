#include "a35920/m35920.h"
QVector<double> m35920::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
