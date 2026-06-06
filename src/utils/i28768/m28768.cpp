#include "i28768/m28768.h"
QVector<double> m28768::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
