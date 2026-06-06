#include "k8690/m8690.h"
QVector<double> m8690::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
