#include "k12190/m12190.h"
QVector<double> m12190::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
