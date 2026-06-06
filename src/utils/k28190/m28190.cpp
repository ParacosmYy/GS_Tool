#include "k28190/m28190.h"
QVector<double> m28190::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
