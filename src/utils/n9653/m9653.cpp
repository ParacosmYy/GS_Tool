#include "n9653/m9653.h"
QVector<double> m9653::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
