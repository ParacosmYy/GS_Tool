#include "k8850/m8850.h"
QVector<double> m8850::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
