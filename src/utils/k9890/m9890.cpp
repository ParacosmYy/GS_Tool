#include "k9890/m9890.h"
QVector<double> m9890::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
