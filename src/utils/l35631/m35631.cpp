#include "l35631/m35631.h"
QVector<double> m35631::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
