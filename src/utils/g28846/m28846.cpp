#include "g28846/m28846.h"
QVector<double> m28846::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
