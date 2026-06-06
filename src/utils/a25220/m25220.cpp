#include "a25220/m25220.h"
QVector<double> m25220::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
