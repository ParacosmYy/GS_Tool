#include "k8290/m8290.h"
QVector<double> m8290::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
