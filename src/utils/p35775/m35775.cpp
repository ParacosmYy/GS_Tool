#include "p35775/m35775.h"
QVector<double> m35775::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
