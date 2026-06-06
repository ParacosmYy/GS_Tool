#include "q35256/m35256.h"
QVector<double> m35256::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
