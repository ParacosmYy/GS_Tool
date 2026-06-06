#include "p28335/m28335.h"
QVector<double> m28335::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
