#include "f30645/m30645.h"
QVector<double> m30645::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
