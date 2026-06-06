#include "c26262/m26262.h"
QVector<double> m26262::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
