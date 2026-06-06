#include "k8230/m8230.h"
QVector<double> m8230::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
