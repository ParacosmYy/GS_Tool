#include "k8930/m8930.h"
QVector<double> m8930::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
