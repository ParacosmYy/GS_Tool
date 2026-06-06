#include "p35555/m35555.h"
QVector<double> m35555::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
