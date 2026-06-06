#include "k8970/m8970.h"
QVector<double> m8970::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
