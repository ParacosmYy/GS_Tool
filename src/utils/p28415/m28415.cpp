#include "p28415/m28415.h"
QVector<double> m28415::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
