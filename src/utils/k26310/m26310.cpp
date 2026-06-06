#include "k26310/m26310.h"
QVector<double> m26310::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
