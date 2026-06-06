#include "f28305/m28305.h"
QVector<double> m28305::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
