#include "k8810/m8810.h"
QVector<double> m8810::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
