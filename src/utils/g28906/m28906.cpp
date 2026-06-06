#include "g28906/m28906.h"
QVector<double> m28906::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
