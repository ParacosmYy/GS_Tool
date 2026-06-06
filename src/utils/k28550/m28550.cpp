#include "k28550/m28550.h"
QVector<double> m28550::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
