#include "k35310/m35310.h"
QVector<double> m35310::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
