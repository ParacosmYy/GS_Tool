#include "g35906/m35906.h"
QVector<double> m35906::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
