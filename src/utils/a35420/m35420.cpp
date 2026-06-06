#include "a35420/m35420.h"
QVector<double> m35420::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
