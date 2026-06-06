#include "k28310/m28310.h"
QVector<double> m28310::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
