#include "b35501/m35501.h"
QVector<double> m35501::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
