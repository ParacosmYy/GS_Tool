#include "a9420/m9420.h"
QVector<double> m9420::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
