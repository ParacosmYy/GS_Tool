#include "p8135/m8135.h"
QVector<double> m8135::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
