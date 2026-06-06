#include "k28370/m28370.h"
QVector<double> m28370::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
