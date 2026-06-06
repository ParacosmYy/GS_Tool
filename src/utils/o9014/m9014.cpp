#include "o9014/m9014.h"
QVector<double> m9014::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
