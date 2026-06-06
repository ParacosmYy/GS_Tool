#include "a35700/m35700.h"
QVector<double> m35700::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
