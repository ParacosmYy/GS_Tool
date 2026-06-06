#include "c35602/m35602.h"
QVector<double> m35602::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
