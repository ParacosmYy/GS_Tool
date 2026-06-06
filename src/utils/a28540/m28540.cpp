#include "a28540/m28540.h"
QVector<double> m28540::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
