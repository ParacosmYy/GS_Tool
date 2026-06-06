#include "k28650/m28650.h"
QVector<double> m28650::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
