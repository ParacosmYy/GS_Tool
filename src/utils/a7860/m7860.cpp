#include "a7860/m7860.h"
QVector<double> m7860::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
