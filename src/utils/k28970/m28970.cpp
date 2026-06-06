#include "k28970/m28970.h"
QVector<double> m28970::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
