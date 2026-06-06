#include "k18870/m18870.h"
QVector<double> m18870::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
