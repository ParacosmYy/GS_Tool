#include "k8750/m8750.h"
QVector<double> m8750::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
